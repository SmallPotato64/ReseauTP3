// Fill out your copyright notice in the Description page of Project Settings.


#include "GoldWidget.h"
#include "TP3Character.h"
#include "Components/TextBlock.h"

void UGoldWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// On récupère le Pawn possédé par le joueur local et on s'y abonne.
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ATP3Character* Character = Cast<ATP3Character>(PC->GetPawn()))
		{
			BoundCharacter = Character;
			BoundCharacter->OnGoldChanged.AddDynamic(this, &UGoldWidget::HandleGoldChanged);
			// Initialisation immédiate avec la valeur courante (utile pour le late join).
			SetGold(Character->GetGold());
		}
	}
}

void UGoldWidget::NativeDestruct()
{
	if (BoundCharacter)
	{
		BoundCharacter->OnGoldChanged.RemoveDynamic(this, &UGoldWidget::HandleGoldChanged);
		BoundCharacter = nullptr;
	}

	Super::NativeDestruct();
}

void UGoldWidget::HandleGoldChanged(int32 NewGold)
{
	SetGold(NewGold);
}

void UGoldWidget::SetGold(int32 NewGold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::AsNumber(NewGold));
	}
}